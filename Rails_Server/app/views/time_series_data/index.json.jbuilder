json.array!(@time_series_data) do |time_series_datum|
  json.extract! time_series_datum, :id, :datetime, :station_id, :lc1, :lc2, :lc3, :lc4
  json.url time_series_datum_url(time_series_datum, format: :json)
end
